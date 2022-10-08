#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CommonMain.h"

// Miniz zip file [de]compression by Rich Geldreich <richgel99@gmail.com>, but we've modified some of 
// the constants so that the exact file format is only readable by our game and not common
// tools such as 7zip, WinRAR, etc.
#include "miniz.h"

// stb_vorbis by Sean T. Barrett - http://nothings.org/
#include "stb_vorbis.h"

#include "Overworld.h"
#include "ItemDefinitions.h"
#include "Platform.h"

/////////// BEGIN GLOBAL EXTERN VARIABLES /////////////
// 
// Every global variable that will be shared among multiple compilation units/*.c files
// needs the extern keyword. When using the extern keyword, the global variable must
// also be assigned a value at the beginning of one and only one *.c file. For these
// variables that will of course be Main.c. Unfortunately simultaneous declaration and
// assignment is not sufficient. (e.g. extern int gVar = 0;)

// Contains data such as number of frames rendered, memory usage, etc.
GAMEPERFDATA gPerformanceData = { 0 };

// The "drawing surface" which we blit to the screen once per frame, 60 times per second.
GAMEBITMAP gBackBuffer = { 0 };

// The smallest, most basic font.
GAMEBITMAP g6x7Font = { 0 };

GAMEBITMAP gPolePigLogo = { 0 };

GAMEBITMAP gLightning01 = { 0 };

// Battle scenes are the 96x96 pictures that serve as the backdrop
// for... battle scenes. We draw a monster over the top of the battle scene.
GAMEBITMAP gBattleScene_Grasslands01 = { 0 };

GAMEBITMAP gBattleScene_Dungeon01 = { 0 };

GAMEBITMAP gMonsterSprite_Slime_001 = { 0 };

GAMEBITMAP gMonsterSprite_Rat_001 = { 0 };

// Consists of both the massive overworld bitmap and also the tilemap.
GAMEMAP gOverworld01 = { 0 };

// Used to track the current and previous game states. The game can only 
// be in one gamestate at a time.
GAMESTATE gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;

GAMESTATE gPreviousGameState = GAMESTATE_OPENINGSPLASHSCREEN;

// Holds the current state of all keyboard and gamepad input, as well as the 
// previous state of input from the last frame. This is used to tell whether
// the player is holding the button(s) down or not.
GAMEINPUT gGameInput = { 0 };

// Noises and music.
GAMESOUND gSoundSplashScreen = { 0 };

GAMESOUND gSoundMenuNavigate = { 0 };

GAMESOUND gSoundMenuChoose = { 0 };

GAMESOUND gMusicOverworld01 = { 0 };

GAMESOUND gMusicDungeon01 = { 0 };

GAMESOUND gMusicBattle01 = { 0 };

GAMESOUND gMusicBattleIntro01 = { 0 };

GAMESOUND gSoundHit01 = { 0 };

GAMESOUND gSoundMiss01 = { 0 };

GAMESOUND gMusicVictoryIntro = { 0 };

GAMESOUND gMusicVictoryLoop = { 0 };

// Yours truly.
HERO gPlayer = { 0 };

// Sound effects and music volume.
float gSFXVolume = 0.5f;

float gMusicVolume = 0.5f;

// Input is briefly disabled during scene transitions, and also when the main game 
// window is out of focus.
bool gInputEnabled;

// These are tiles that the player can walk normally on. e.g. NOT water or lava or walls.
uint8_t gPassableTiles[11] = { 0 };

// Imagine the camera is 50 feet up the sky looking straight down over the player.
// Knowing the position of the camera is necessary to properly pan the overworld map as
// the player walks.
POINT gCamera = { 0 };

// This will be -1 if there is no gamepad plugged in. It will
// be 0 if a gamepad is plugged into the first port.
int gGamepadID = -1;

#ifdef _WIN32

// Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain
BOOL gGameIsRunning = TRUE;

#endif

// Draw some debug statistics when pressing the F1 key.
void DrawDebugInfo(void)
{
    char DebugTextBuffer[64] = { 0 };

    DrawWindow(
        0, 
        0, 
        144, 
        97,
        NULL, 
        &(COLOR_NES_BLACK), 
        NULL, 
        WINDOW_FLAG_OPAQUE);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "FPS:     %.01f (%.01f)", 
        gPerformanceData.CookedFPSAverage, 
        gPerformanceData.RawFPSAverage);  
    
    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 0),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "Timer:   %.02f/%.02f/%.02f", 
        (gPerformanceData.MinimumTimerResolution / 10000.0f),
        (gPerformanceData.MaximumTimerResolution / 10000.0f),
        (gPerformanceData.CurrentTimerResolution / 10000.0f));

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font,        
        0, 
        (8 * 1),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "Handles: %lu", 
        (long)gPerformanceData.HandleCount);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 2),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "Memory:  %zu KB", 
        gPerformanceData.MemoryUsage / 1024);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 3),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "CPU:     %.02f%%", 
        gPerformanceData.CPUPercent);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 4),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "FramesT: %llu", 
        (unsigned long long int)gPerformanceData.TotalFramesRendered);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 5),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "ScreenXY:%ld,%ld", 
        (long)gPlayer.ScreenPos.x,
        (long)gPlayer.ScreenPos.y);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 6),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "WorldXY: %ld,%ld", 
        (long)gPlayer.WorldPos.x,
        (long)gPlayer.WorldPos.y);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 7),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "CameraXY:%ld,%ld", 
        (long)gCamera.x,
        (long)gCamera.y);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 8),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "Movement:%u", 
        gPlayer.MovementRemaining);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 9),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer, 
        sizeof(DebugTextBuffer), 
        "Steps:   %u", 
        gPlayer.StepsTaken);

    BlitStringEx(
        DebugTextBuffer, 
        &g6x7Font, 
        0, 
        (8 * 10),
        255,
        255,
        255,
        0,
        0);

    sprintf_s(
        DebugTextBuffer,
        sizeof(DebugTextBuffer),
        "Input:   %s",
        gInputEnabled ? "ON" : "OFF");

    BlitStringEx(
        DebugTextBuffer,
        &g6x7Font,
        0,
        (8 * 11),
        255,
        255,
        255,
        0,
        0);

    // Draw the tile values around the player so we can verify which tiles we should be and should not be allowed to walk on.
 
    if (gCurrentGameState == GAMESTATE_OVERWORLD)
    {
        // What is the value of the tile that the player is currently standing on?
        _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16], DebugTextBuffer, sizeof(DebugTextBuffer), 10);

        BlitStringEx(
            DebugTextBuffer, 
            &g6x7Font, 
            gPlayer.ScreenPos.x + 5, 
            gPlayer.ScreenPos.y + 4,
            255,
            255,
            255,
            0,
            BLIT_FLAG_TEXT_SHADOW);

        if (gPlayer.ScreenPos.y >= 16)
        {
            // What is the value of the tile above the player?
            _itoa_s(gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) - 1][gPlayer.WorldPos.x / 16], DebugTextBuffer, sizeof(DebugTextBuffer), 10);

            BlitStringEx(
                DebugTextBuffer, 
                &g6x7Font, 
                gPlayer.ScreenPos.x + 5, 
                (gPlayer.ScreenPos.y - 16) + 4,
                255,
                255,
                255,
                0,
                BLIT_FLAG_TEXT_SHADOW);
        }

        if (gPlayer.ScreenPos.x < (GAME_RES_WIDTH - 16))
        {
            // What is the value of the tile to the right of the player?
            _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) + 1], DebugTextBuffer, sizeof(DebugTextBuffer), 10);

            BlitStringEx(
                DebugTextBuffer, 
                &g6x7Font, 
                (gPlayer.ScreenPos.x + 16) + 5, 
                gPlayer.ScreenPos.y + 4,                
                255,
                255,
                255,
                0,
                BLIT_FLAG_TEXT_SHADOW);
        }

        if (gPlayer.ScreenPos.x >= 16)
        {
            // What is the value of the tile to left of the player?
            _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) - 1], DebugTextBuffer, sizeof(DebugTextBuffer), 10);

            BlitStringEx(
                DebugTextBuffer, 
                &g6x7Font, 
                (gPlayer.ScreenPos.x - 16) + 5, 
                gPlayer.ScreenPos.y + 4,
                255,
                255,
                255,
                0,
                BLIT_FLAG_TEXT_SHADOW);
        }

        if (gPlayer.ScreenPos.y <= GAME_RES_HEIGHT - 32)
        {
            // What is the value of the tile below the player?
            _itoa_s(gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) + 1][gPlayer.WorldPos.x / 16], DebugTextBuffer, sizeof(DebugTextBuffer), 10);

            BlitStringEx(
                DebugTextBuffer, 
                &g6x7Font,
                gPlayer.ScreenPos.x + 5, 
                (gPlayer.ScreenPos.y + 16) + 4,
                255,
                255,
                255,
                0,
                BLIT_FLAG_TEXT_SHADOW);
        }
    }
}

// This resets all necessary global variables. Call this once during game 
// startup, and whenever the player chooses to start a new game from scratch.
void ResetEverythingForNewGame(void)
{
    LogMessageA(LL_INFO, "[%s] Resetting everything for a new game.", __FUNCTION__);

    StopMusic();

    gPreviousGameState = GAMESTATE_OPENINGSPLASHSCREEN;

    gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;

    gCamera.x = 0;

    gCamera.y = 592;

#ifdef _WIN32

    gGameIsRunning = true;

    // Not sure why this is done here in Windows.
    // On Linux, gGamepadID is a file descriptor that gets set at program start.
    // It's valid until an ioctl fails, at which point it gets closed and set to -1.
    // If the user starts a new game, the gamepad is still valid.
    gGamepadID = -1;

#endif

    gPassableTiles[0] = TILE_GRASS_01;

    gPassableTiles[1] = TILE_PORTAL_01;

    gPassableTiles[2] = TILE_BRICK_01;

    gPassableTiles[3] = TILE_SNOW_01;

    gPassableTiles[4] = TILE_SAND_01;

    gPassableTiles[5] = TILE_TREES_01;

    gPassableTiles[6] = TILE_TREES_02;

    gPassableTiles[7] = TILE_TREES_03;

    gPassableTiles[8] = TILE_SWAMP_01;

    gPassableTiles[9] = TILE_HOUSE_01;

    gPassableTiles[10] = TILE_BRIDGE_01;


    
    gOverworldArea = (GAMEAREA)
    {
        .Name = "The World",
        .Area = (RECT){.left = 0, .top = 0, .right = 3840, .bottom = 2400 },
        .Music = &gMusicOverworld01
    };

    gHomeGameArea = (GAMEAREA)
    {
        .Name = "HomeSweetHome",
        .Area = (RECT){.left = 3856, .top = 0, .right = 4240, .bottom = 240 },
        .Music = &gMusicDungeon01
    };

    gCurrentArea = gOverworldArea;


    // This is the player's house at the beginning of the game.
    gPortal001 = (PORTAL)
    {
        .DestinationArea = gHomeGameArea,                   // The area you will be in after teleport
        .CameraPos = (POINT){.x = 3856, .y = 0 },          // The camera's position after teleport
        .ScreenDestination = (POINT){.x = 64, .y = 32 },   // Screen coords after teleport
        .WorldDestination = (POINT){.x = 3920, .y = 32},   // Player's world position after teleport
        .WorldPos = (POINT){ .x = 64, .y = 736 }           // The world position of the origin portal
    };        

    // This is the door from the player's house back outside to the overworld.
    gPortal002 = (PORTAL)
    {
        .DestinationArea = gOverworldArea,
        .CameraPos = (POINT){ .x = 0, .y = 592 },
        .ScreenDestination = (POINT){ .x = 64, .y = 144 },
        .WorldDestination = (POINT){ .x = gPortal001.WorldPos.x, .y = gPortal001.WorldPos.y },
        .WorldPos = (POINT){ .x = 3920, .y = 32 } 
    };

    // Add all portals to an array.

    gPortals[0] = gPortal001;

    gPortals[1] = gPortal002;




	
	

    gPlayer.Active = false;

    memset(gPlayer.Name, 0, sizeof(gPlayer.Name));

    gPlayer.HasPlayerMovedSincePortal = false;

    gPlayer.MovementRemaining = 0;

    gPlayer.StepsSinceLastRandomMonsterEncounter = 0;

    gPlayer.StepsTaken = 0;
	
    gPlayer.MaxHP = 10;

    gPlayer.HP = 10;
    
    gPlayer.Money = 0;

    gPlayer.MP = 0;             // Magic points (start the game with no magic)

    gPlayer.XP = 0;             // Experience points

    gPlayer.Strength = 1;       // How much physical damage you deal

    gPlayer.Dexterity = 1;      // Chance to hit with weapons

    gPlayer.Luck = 0;           // Stat that adds a little something extra

    gPlayer.Intelligence = 1;   // Spell damage and chance to hit

    gPlayer.Evasion = 1;        // Lowers enemy's chance to hit you, imrpoves your run chance

    gPlayer.ScreenPos.x = 80;

    gPlayer.ScreenPos.y = 160;

    gPlayer.WorldPos.x = 80;

    gPlayer.WorldPos.y = 752;

    gPlayer.CurrentArmor = SUIT_0;

    gPlayer.Direction = DOWN;

    // 90 = 10% chance, 80 = 20% chance, etc. 100 = 0% chance.
    gPlayer.RandomEncounterPercentage = 50;

    // The 0th inventory slot will always hold the player's equipped armor.
    // The 1st inventory slot will always hold the player's equipped weapon.
    // The 2nd inventory slot will always hold the player's equipped shield.
    // When the player gets a better item, it will be swapped out with another inventory slot.

    // Clear the player's inventory, then add the starter items.
    memset(gPlayer.Inventory, 0, sizeof(gPlayer.Inventory));

    gPlayer.Inventory[0] = gArmor00;
    
    gPlayer.Inventory[1] = gWeapon00;

    // no shield

    gPlayer.Inventory[3] = gPotion00;

    return;    
}

// If WINDOW_FLAG_HORIZONTALLY_CENTERED is specified, the x coordinate is ignored and may be zero.
// If WINDOW_FLAG_VERTICALLY_CENTERED is specified, the y coordinate is ignored and may be zero.
// BackgroundColor is ignored and may be NULL if WINDOW_FLAG_OPAQUE is not set.
// BorderColor is ignored and may be NULL if WINDOW_FLAG_BORDERED is not set.
// Either the BORDERED or the OPAQUE flag needs to be set, or both, or else the window would just be
// transparent and invisible. 
// The window border will cut into the inside of the window area.

void DrawWindow(
    int x,
    int y,
    int Width,
    int Height,
    PIXEL32* BorderColor,
    PIXEL32* BackgroundColor,
    PIXEL32* ShadowColor,
    uint32_t Flags)
{
    if (Flags & WINDOW_FLAG_HORIZONTALLY_CENTERED)
    {
        x = (GAME_RES_WIDTH / 2) - (Width / 2);
    }

    if (Flags & WINDOW_FLAG_VERTICALLY_CENTERED)
    {
        y = (GAME_RES_HEIGHT / 2) - (Height / 2);
    }

    ASSERT((x + Width <= GAME_RES_WIDTH) && (y + Height <= GAME_RES_HEIGHT), "Window is off the screen!");
    
    ASSERT((Flags & WINDOW_FLAG_BORDERED) || (Flags & WINDOW_FLAG_OPAQUE), "Window must have either the BORDERED or the OPAQUE flags (or both) set!");

    int StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

    if (Flags & WINDOW_FLAG_OPAQUE)
    {
        ASSERT(BackgroundColor != NULL, "WINDOW_FLAG_OPAQUE is set but BackgroundColor is NULL!");

        for (int Row = 0; Row < Height; Row++)
        {
            int MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row);

            // If the user wants rounded corners, don't draw the first and last pixels on the first and last rows.
            // Get a load of this sweet ternary action:
            for (int Pixel = ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) && (Row == 0 || Row == Height - 1)) ? 1 : 0; 
                Pixel < Width - ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) && (Row == 0 || Row == Height - 1)) ? 1 : 0;
                Pixel++)
            {
                if ((Flags & WINDOW_FLAG_ALPHABLEND) && (BackgroundColor->Colors.Alpha < 255))
                {
                    PIXEL32 BackgroundPixel = { 0 };

                    PIXEL32 BlendedPixel = { 0 };

                    // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                    memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, sizeof(PIXEL32));

                    // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                    BlendedPixel.Colors.Blue  = BackgroundColor->Colors.Blue * BackgroundColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BackgroundColor->Colors.Alpha) / 255;

                    BlendedPixel.Colors.Green = BackgroundColor->Colors.Green * BackgroundColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BackgroundColor->Colors.Alpha) / 255;

                    BlendedPixel.Colors.Red   = BackgroundColor->Colors.Red * BackgroundColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BackgroundColor->Colors.Alpha) / 255;

                    memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, &BlendedPixel, sizeof(PIXEL32));
                }
                else
                {
                    memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BackgroundColor, sizeof(PIXEL32));
                }
            }            
        }
    }

    if (Flags & WINDOW_FLAG_BORDERED)
    {
        ASSERT(BorderColor != NULL, "WINDOW_FLAG_BORDERED is set but BorderColor is NULL!");       

        // Draw the top of the border.
        int MemoryOffset = StartingScreenPixel;

        for (int Pixel = ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) ? 1 : 0); 
            Pixel < Width - ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) ? 1 : 0);
            Pixel++)
        {
            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (BorderColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = BorderColor->Colors.Blue * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = BorderColor->Colors.Green * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = BorderColor->Colors.Red * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BorderColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
            }
        }

        // Draw the bottom of the border.
        MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * (Height - 1));

        for (int Pixel = ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) ? 1 : 0);
            Pixel < Width - ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) ? 1 : 0);
            Pixel++)
        {
            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (BorderColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue  = BorderColor->Colors.Blue * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = BorderColor->Colors.Green * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red   = BorderColor->Colors.Red * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BorderColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, BorderColor, sizeof(PIXEL32));
            }            
        }

        // Draw one pixel on the left side and the right for each row of the border, from the top down.
        for (int Row = 1; Row < Height - 1; Row++)
        {
            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (BorderColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row);

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = BorderColor->Colors.Blue * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = BorderColor->Colors.Green * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = BorderColor->Colors.Red * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BorderColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, &BlendedPixel, sizeof(PIXEL32));

                MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row) + (Width - 1);

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = BorderColor->Colors.Blue * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = BorderColor->Colors.Green * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BorderColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = BorderColor->Colors.Red * BorderColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BorderColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row);

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));

                MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row) + (Width - 1);

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BorderColor, sizeof(PIXEL32));
            }
        }
        
        // Recursion ahead!
        // If the user wants a thick window, just draw a smaller concentric bordered window inside the existing window.
        if (Flags & WINDOW_FLAG_THICK)
        {
            if (Flags & WINDOW_FLAG_ALPHABLEND)
            {
                DrawWindow(x + 1, y + 1, Width - 2, Height - 2, BorderColor, NULL, NULL, WINDOW_FLAG_BORDERED | WINDOW_FLAG_ALPHABLEND);
            }
            else
            {
                DrawWindow(x + 1, y + 1, Width - 2, Height - 2, BorderColor, NULL, NULL, WINDOW_FLAG_BORDERED);
            }
        }
    }

    // TODO: If a window was placed at the edge of the screen, the shadow effect might attempt
    // to draw off-screen and crash! i.e. make sure there's room to draw the shadow before attempting!
    if (Flags & WINDOW_FLAG_SHADOW)
    {
        ASSERT(ShadowColor != NULL, "WINDOW_FLAG_SHADOW is set but ShadowColor is NULL!");        

        // Draw the bottom of the shadow.
        int MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Height);

        for (int Pixel = 1; 
            Pixel < Width + ((Flags & WINDOW_FLAG_ROUNDED_CORNERS) ? 0 : 1); 
            Pixel++)
        {
            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (ShadowColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = ShadowColor->Colors.Blue * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = ShadowColor->Colors.Green * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = ShadowColor->Colors.Red * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - ShadowColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset + Pixel, ShadowColor, sizeof(PIXEL32));
            }            
        }

        // Draw one pixel on the right side for each row of the border, from the top down.
        for (int Row = 1; Row < Height; Row++)
        {
            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * Row) + Width;

            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (ShadowColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = ShadowColor->Colors.Blue * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = ShadowColor->Colors.Green * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = ShadowColor->Colors.Red * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - ShadowColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, ShadowColor, sizeof(PIXEL32));
            }
        }

        // Draw one shadow pixel in the bottom-right corner to compensate for rounded corner.
        if (Flags & WINDOW_FLAG_ROUNDED_CORNERS) 
        {
            MemoryOffset = StartingScreenPixel - (GAME_RES_WIDTH * (Height - 1)) + (Width - 1);

            if ((Flags & WINDOW_FLAG_ALPHABLEND) && (ShadowColor->Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy(&BackgroundPixel, (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue = ShadowColor->Colors.Blue * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = ShadowColor->Colors.Green * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - ShadowColor->Colors.Alpha) / 255;

                BlendedPixel.Colors.Red = ShadowColor->Colors.Red * ShadowColor->Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - ShadowColor->Colors.Alpha) / 255;

                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                memcpy((PIXEL32*)gBackBuffer.Memory + MemoryOffset, ShadowColor, sizeof(PIXEL32));
            }
        }
    }
}

// Draw HUD at the top-left, unless the player is standing there, in which case draw it at the top-right.
// Both Overworld gamestate and Battle gamestate need this function.
void DrawPlayerStatsWindow(int AlphaAdjust, int WindowShakeX, int WindowShakeY)
{
    char TextBuffer[32] = { 0 };

    // Exactly enough width to fit an 8-character name with 1-pixel padding on each side.
    int WindowWidth = 53;

    int WindowHeight = 64;

    // Center the player's name depending on the name's length.
    // WindowWidth - 4 is to accomodate for the thick borders.
    int PlayerNameOffset = (gPlayer.ScreenPos.x <= 48 && gPlayer.ScreenPos.y <= WindowHeight) ?
        (326 + (((WindowWidth - 4) / 2) - ((int)(strlen(gPlayer.Name) * 6) / 2))) :
        (11 + (((WindowWidth - 4) / 2) - ((int)(strlen(gPlayer.Name) * 6) / 2)));

    // Draw the main player stats window top left, unless player is standing underneath that area,
    // in which case draw it top right.
    DrawWindow(
        (gPlayer.ScreenPos.x <= 48 && gPlayer.ScreenPos.y <= WindowHeight) ? (GAME_RES_WIDTH - WindowWidth - 8) + WindowShakeX : 8 + WindowShakeX,
        8 + WindowShakeY,
        WindowWidth,
        WindowHeight,
        (gPlayer.HP <= (gPlayer.MaxHP * 0.2f)) ? 
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0x00, .Colors.Blue = 0x00 } :
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
        (WindowShakeX || WindowShakeY) ?
        &(PIXEL32) { .Colors.Alpha = 0xFF, .Colors.Red = 0xFF, .Colors.Green = 0x00, .Colors.Blue = 0x00 } :
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0x00, .Colors.Green = 0x00, .Colors.Blue = 0x00 },
        &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + AlphaAdjust), 0))), .Colors.Red = 0x40, .Colors.Green = 0x40, .Colors.Blue = 0x40 },
            WINDOW_FLAG_SHADOW | WINDOW_FLAG_BORDERED | WINDOW_FLAG_THICK | WINDOW_FLAG_OPAQUE | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_ALPHABLEND);

    BlitStringEx(
        gPlayer.Name,
        &g6x7Font,
        PlayerNameOffset,
        11,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        0xFC,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    sprintf_s(TextBuffer, sizeof(TextBuffer), "HP:%d", gPlayer.HP);

    BlitStringEx(
        TextBuffer,
        &g6x7Font,
        (gPlayer.ScreenPos.x <= 48 && gPlayer.ScreenPos.y <= WindowHeight) ? 326 : 11,
        21,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        0xFC,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    sprintf_s(TextBuffer, sizeof(TextBuffer), "MP:%d", gPlayer.MP);

    BlitStringEx(
        TextBuffer,
        &g6x7Font,
        (gPlayer.ScreenPos.x <= 48 && gPlayer.ScreenPos.y <= WindowHeight) ? 326 : 11,
        21 + (8 * 1),
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        0xFC,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    sprintf_s(TextBuffer, sizeof(TextBuffer), "GP:%d", gPlayer.Money);

    BlitStringEx(
        TextBuffer,
        &g6x7Font,
        (gPlayer.ScreenPos.x <= 48 && gPlayer.ScreenPos.y <= WindowHeight) ? 326 : 11,
        21 + (8 * 2),
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        (gPlayer.HP <= gPlayer.MaxHP * 0.2f) ? 0 : 0xFC,
        0xFC,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
}

// TODO: What Flags do we need here?
void Blit32BppBitmapEx(
    GAMEBITMAP* GameBitmap, 
    int x,
    int y,
    int BlueAdjust,
    int GreenAdjust,
    int RedAdjust,
    int AlphaAdjust,
    uint32_t Flags)
{
    int StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

    int StartingBitmapPixel = ((GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - \
        GameBitmap->BitmapInfo.bmiHeader.biWidth);

    int MemoryOffset = 0;

    int BitmapOffset = 0;

    PIXEL32 BitmapPixel = { 0 };    

    for (int YPixel = 0; YPixel < GameBitmap->BitmapInfo.bmiHeader.biHeight; YPixel++)
    {
        for (int XPixel = 0; XPixel < GameBitmap->BitmapInfo.bmiHeader.biWidth; XPixel++)
        {
            // Calculate the offset into the backbuffer where this pixel will be drawn.
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            // Calculate the offset into the bitmap image that we are drawing to the backbuffer.
            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            // Load the single pixel from the bitmap image.
            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            // Clamp between 0 and 255 // min(upper, max(x, lower))            
            BitmapPixel.Colors.Red   = (uint8_t)min(255, max((BitmapPixel.Colors.Red + RedAdjust), 0));

            BitmapPixel.Colors.Green = (uint8_t)min(255, max((BitmapPixel.Colors.Green + GreenAdjust), 0));

            BitmapPixel.Colors.Blue  = (uint8_t)min(255, max((BitmapPixel.Colors.Blue + BlueAdjust), 0));

            BitmapPixel.Colors.Alpha = (uint8_t)min(255, max((BitmapPixel.Colors.Alpha + AlphaAdjust), 0));

            // Perform alpha blending if requested. But don't waste time alpha blending if the pixel is opaque.
            if ((Flags & BLIT_FLAG_ALPHABLEND) && (BitmapPixel.Colors.Alpha < 255))
            {
                PIXEL32 BackgroundPixel = { 0 };

                PIXEL32 BlendedPixel = { 0 };

                // If we are alpha blending, pick up the background pixel first so we know what color we are blending with.
                memcpy_s(&BackgroundPixel, sizeof(PIXEL32), (PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32));

                // Shifting right by 8 bits (>> 8) is faster, but dividing by 255 is more color-accurate.
                BlendedPixel.Colors.Blue  = BitmapPixel.Colors.Blue * BitmapPixel.Colors.Alpha / 255 + BackgroundPixel.Colors.Blue * (255 - BitmapPixel.Colors.Alpha) / 255;

                BlendedPixel.Colors.Green = BitmapPixel.Colors.Green * BitmapPixel.Colors.Alpha / 255 + BackgroundPixel.Colors.Green * (255 - BitmapPixel.Colors.Alpha) / 255;

                BlendedPixel.Colors.Red   = BitmapPixel.Colors.Red * BitmapPixel.Colors.Alpha / 255 + BackgroundPixel.Colors.Red * (255 - BitmapPixel.Colors.Alpha) / 255;

                memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BlendedPixel, sizeof(PIXEL32));
            }
            else
            {
                if (BitmapPixel.Colors.Alpha > 0)
                {
                    memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
                }
            }
        }
    }
}

void BlitStringEx(
    char* String,
    GAMEBITMAP* FontSheet,
    int x,
    int y,
    int BlueAdjust,
    int GreenAdjust,
    int RedAdjust,
    int AlphaAdjust,
    uint32_t Flags)
{
    // Map any char value to an offset dictated by the g6x7Font ordering.
    // 0xab and 0xbb are extended ASCII characters that look like double angle brackets.
    // We use them as a cursor in menus.
    static int FontCharacterPixelOffset[] = {
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //     !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
        94,64,87,66,67,68,70,85,72,73,71,77,88,74,91,92,52,53,54,55,56,57,58,59,60,61,86,84,89,75,90,93,
    //  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
        65,0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,80,78,81,69,76,
    //  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  ..
        62,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,82,79,83,63,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. bb .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ab .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,96,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,95,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,
    //  .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. F2 .. .. .. .. .. .. .. .. .. .. .. .. ..
        93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,97,93,93,93,93,93,93,93,93,93,93,93,93,93
    };

    int CharWidth = FontSheet->BitmapInfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;

    int CharHeight = FontSheet->BitmapInfo.bmiHeader.biHeight;

    int BytesPerCharacter = (CharWidth * CharHeight * (FontSheet->BitmapInfo.bmiHeader.biBitCount / 8));

    int StringLength = (int)strlen(String);

    GAMEBITMAP StringBitmap = { 0 };

    StringBitmap.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    StringBitmap.BitmapInfo.bmiHeader.biHeight = CharHeight;

    StringBitmap.BitmapInfo.bmiHeader.biWidth = CharWidth * StringLength;

    StringBitmap.BitmapInfo.bmiHeader.biPlanes = 1;

    StringBitmap.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    StringBitmap.Memory = calloc(StringLength, BytesPerCharacter);

    for (int Character = 0; Character < StringLength; Character++)
    {
        int StartingFontSheetPixel = 0;

        int FontSheetOffset = 0;

        int StringBitmapOffset = 0;

        PIXEL32 FontSheetPixel = { 0 };

        StartingFontSheetPixel = (FontSheet->BitmapInfo.bmiHeader.biWidth * FontSheet->BitmapInfo.bmiHeader.biHeight) - \
            FontSheet->BitmapInfo.bmiHeader.biWidth + (CharWidth * FontCharacterPixelOffset[(uint8_t)String[Character]]);

        for (int YPixel = 0; YPixel < CharHeight; YPixel++)
        {
            for (int XPixel = 0; XPixel < CharWidth; XPixel++)
            {
                FontSheetOffset = StartingFontSheetPixel + XPixel - (FontSheet->BitmapInfo.bmiHeader.biWidth * YPixel);

                StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.BitmapInfo.bmiHeader.biWidth * StringBitmap.BitmapInfo.bmiHeader.biHeight) - \
                    StringBitmap.BitmapInfo.bmiHeader.biWidth) + XPixel - (StringBitmap.BitmapInfo.bmiHeader.biWidth) * YPixel;


                // NOTE: memcpy_s is safer but is much slower.
                //memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)FontSheet->Memory + FontSheetOffset, sizeof(PIXEL32));
                memcpy(&FontSheetPixel, (PIXEL32*)FontSheet->Memory + FontSheetOffset, sizeof(PIXEL32));
                
                if (FontSheetPixel.Colors.Alpha > 0)
                {                    
                    //memcpy_s((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, sizeof(PIXEL32), &FontSheetPixel, sizeof(PIXEL32));                    
                    memcpy((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, &FontSheetPixel, sizeof(PIXEL32));
                }                
            }
        }
    }

    if (Flags & BLIT_FLAG_TEXT_SHADOW)
    {
        // If alpha blending was requested, we can perform the shadow effect
        // using the alpha channel.
        // If the caller requested a text shadow but no alpha blending,
        // we must use the color adjustments to produce a shadow.
        if (Flags & BLIT_FLAG_ALPHABLEND)
        {
            Blit32BppBitmapEx(
                &StringBitmap,
                x,
                y + 1,
                BlueAdjust,
                GreenAdjust,
                RedAdjust,
                AlphaAdjust - 160,
                Flags ^ BLIT_FLAG_TEXT_SHADOW);
        }
        else
        {
            Blit32BppBitmapEx(
                &StringBitmap,
                x,
                y + 1,
                BlueAdjust - 160,
                GreenAdjust - 160,
                RedAdjust - 160,
                0,
                Flags ^ BLIT_FLAG_TEXT_SHADOW);
        }
    }

    Blit32BppBitmapEx(&StringBitmap, x, y, BlueAdjust, GreenAdjust, RedAdjust, AlphaAdjust, Flags);

    if (StringBitmap.Memory)
    {
        free(StringBitmap.Memory);
    }
}

// Draws a subsection of a background across the entire screen.
// The background may be an entire overworld map which may be much larger than the screen.
// Uses gCamera to control which part of the background gets drawn to the screen.
// The camera is panned around based on the character's movement. E.g., when the player
// walks toward the edge of the screen, the camera gets pushed in that direction.
// Because of the use of SIMD in this function, it's important that the background and thus
// the game's resolution be a multiple of 4 (SSE) or 8 (AVX).
void BlitBackground(GAMEBITMAP* GameBitmap, int ColorAdjust)
{
    int StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH);

    int StartingBitmapPixel = ((GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - \
        GameBitmap->BitmapInfo.bmiHeader.biWidth) + gCamera.x - (GameBitmap->BitmapInfo.bmiHeader.biWidth * gCamera.y);

    int MemoryOffset = 0;

    int BitmapOffset = 0;

#ifdef __AVX2__
    // We go 8 pixels at a time, SIMD-style.    

    __m256i BitmapOctoPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 8)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            // Load 256 bits (8 pixels) from memory into register YMMx
            BitmapOctoPixel = _mm256_load_si256((const __m256i*)((PIXEL32*)GameBitmap->Memory + BitmapOffset));
            //        AARRGGBBAARRGGBB-AARRGGBBAARRGGBB-AARRGGBBAARRGGBB-AARRGGBBAARRGGBB
            // YMM0 = FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1

            // Blow the 256-bit vector apart into two separate 256-bit vectors Half1 and Half2, 
            // each containing 4 pixels, where each pixel is now 16 bits instead of 8.            

            __m256i Half1 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 0));
            //        AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB
            // YMM0 = 00FF005B006E00E1-00FF005B006E00E1-00FF005B006E00E1-00FF005B006E00E1

            // Add the brightness adjustment to each 16-bit element
            Half1 = _mm256_add_epi16(Half1, _mm256_set1_epi16((short)ColorAdjust));
            // YMM0 = 0000FF5CFF6FFFE2-0000FF5CFF6FFFE2-0000FF5CFF6FFFE2-0000FF5CFF6FFFE2

            // Do the same for Half2 that we just did for Half1.
            __m256i Half2 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 1));

            Half2 = _mm256_add_epi16(Half2, _mm256_set1_epi16((short)ColorAdjust));

            // Now we need to reassemble the two halves back into a single 256-bit group of 8 pixels.
            // _mm256_packus_epi16(a,b) takes the 16-bit signed integers in the 256-bit vectors a and b
            // and converts them to a 256-bit vector of 8-bit unsigned integers. The result contains the
            // first 8 integers from a, followed by the first 8 integers from b, followed by the last 8
            // integers from a, followed by the last 8 integers from b.
            // Values that are out of range are set to 0 or 255.
            __m256i Recombined = _mm256_packus_epi16(Half1, Half2);

            BitmapOctoPixel = _mm256_permute4x64_epi64(Recombined, _MM_SHUFFLE(3, 1, 2, 0));

            // Store the result to the global back buffer.
            _mm256_store_si256((__m256i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), BitmapOctoPixel);
        }
    }

#else
    // We go 1 pixel at a time, no SIMD.

    PIXEL32 BitmapPixel = { 0 };

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel++)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            // Clamp between 0 and 255
            // min(upper, max(x, lower))
            BitmapPixel.Colors.Red   = (uint8_t) min(255, max((BitmapPixel.Colors.Red + ColorAdjust), 0));

            BitmapPixel.Colors.Green = (uint8_t) min(255, max((BitmapPixel.Colors.Green + ColorAdjust), 0));

            BitmapPixel.Colors.Blue  = (uint8_t) min(255, max((BitmapPixel.Colors.Blue + ColorAdjust), 0));

            memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
        }
    }
#endif
}

// This function should only be called from the LoadAssetFromArchive function.
// It takes raw bitmap data that has been extracted to the heap and turns it into
// a usable GAMEBITMAP structure.
static uint32_t Load32BppBitmapFromMemory(void* Buffer, GAMEBITMAP* GameBitmap)
{
    uint32_t Error = ERROR_SUCCESS;    

    uint16_t BitmapHeader = 0;

    uint32_t PixelDataOffset = 0;    

    memcpy(&BitmapHeader, Buffer, sizeof(uint16_t));

    if (BitmapHeader != 0x4d42) // "BM" backwards
    {
        Error = ERROR_INVALID_DATA;

        goto Exit;
    }

    memcpy(&PixelDataOffset, (char*)Buffer + 0xA, sizeof(uint32_t));

    memcpy(&GameBitmap->BitmapInfo.bmiHeader, (char*)Buffer + 0xE, sizeof(BITMAPINFOHEADER));

    GameBitmap->Memory = (char*)Buffer + PixelDataOffset;

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Loading successful.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Loading failed! Error 0x%08lx!", __FUNCTION__, Error);
    }

    return(Error);
}

// A compressed *.wav file has been extracted from the assets file and now resides
// in heap memory. This function parses that memory and populates a GAMESOUND data structure.
// This function should only be called from LoadAssetFromArchive.
static uint32_t LoadWavFromMemory(void* Buffer, GAMESOUND* GameSound)
{
    uint32_t Error = ERROR_SUCCESS;    

    uint32_t RIFF = 0;

    uint16_t DataChunkOffset = 0;

    uint32_t DataChunkSearcher = 0;

    bool DataChunkFound = false;

    uint32_t DataChunkSize = 0;

    memcpy(&RIFF, Buffer, sizeof(uint32_t));

    if (RIFF != 0x46464952) // "RIFF" backwards
    {
        Error = ERROR_FILE_INVALID;

        LogMessageA(LL_ERROR, "[%s] First four bytes of memory buffer are not 'RIFF'!", __FUNCTION__, Error);

        goto Exit;
    }

    // 20 bytes into a wav file, there is a WAVEFORMATEX data structure.
    memcpy(&GameSound->WaveFormat, (uint8_t *)Buffer + 20, sizeof(WAVEFORMATEX));

    if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
        (GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) ||
        (GameSound->WaveFormat.wBitsPerSample != 16))
    {
        Error = ERROR_DATATYPE_MISMATCH;

        LogMessageA(LL_ERROR, "[%s] This wav data in the memory buffer did not meet the format requirements! Only PCM format, 44.1KHz, 16 bits per sample wav files are supported. 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    // We search for the data chunk, which is an indeterminite number of bytes into the file/buffer.
    while (DataChunkFound == false)
    {
        memcpy(&DataChunkSearcher, (uint8_t *)Buffer + DataChunkOffset, sizeof(uint32_t));

        if (DataChunkSearcher == 0x61746164) // 'data', backwards
        {
            DataChunkFound = true;

            break;
        }
        else
        {
            DataChunkOffset += 4;
        }

        if (DataChunkOffset > 256)
        {
            Error = ERROR_DATATYPE_MISMATCH;

            LogMessageA(LL_ERROR, "[%s] Data chunk not found within first 256 bytes of the memory buffer! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    memcpy(&DataChunkSize, (uint8_t *)Buffer + DataChunkOffset + 4, sizeof(uint32_t));

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = DataChunkSize;

    GameSound->Buffer.pAudioData = (uint8_t *)Buffer + DataChunkOffset + 8;

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Successfully loaded wav from memory.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_ERROR, "[%s] Failed to load wav from memory! Error: 0x%08lx!", __FUNCTION__, Error);
    }

    return(Error);
}

// This function should only be called from the LoadAssetFromArchive function.
// It takes raw OGG Vorbis data that has been extracted to the heap and turns it into
// a usable GAMESOUND structure.
static uint32_t LoadOggFromMemory(void* Buffer, uint32_t BufferSize, GAMESOUND* GameSound)
{
    uint32_t Error = ERROR_SUCCESS;    

    int SamplesDecoded = 0;

    int Channels = 0;

    int SampleRate = 0;

    short* DecodedAudio = NULL;    

    SamplesDecoded = stb_vorbis_decode_memory(Buffer, (int)BufferSize, &Channels, &SampleRate, &DecodedAudio);

    if (SamplesDecoded < 1)
    {
        Error = ERROR_BAD_COMPRESSION_BUFFER;

        LogMessageA(LL_ERROR, "[%s] stb_vorbis_decode_memory failed with 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    GameSound->WaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    GameSound->WaveFormat.nChannels = (uint16_t)Channels;

    GameSound->WaveFormat.nSamplesPerSec = SampleRate;

    GameSound->WaveFormat.nAvgBytesPerSec = GameSound->WaveFormat.nSamplesPerSec * GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.nBlockAlign = GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.wBitsPerSample = 16;

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = SamplesDecoded * GameSound->WaveFormat.nChannels * 2;

    GameSound->Buffer.pAudioData = (const uint8_t*)DecodedAudio;

Exit:

    return(Error);
}

// This function should only be called from the LoadAssetFromArchive function.
// It takes raw tilemap data that has been extracted to the heap and turns it into
// a usable TILEMAP structure. This tilemap data is an XML document that was generated
// by the map editing application "Tiled."
static uint32_t LoadTilemapFromMemory(void* Buffer, uint32_t BufferSize, TILEMAP* TileMap)
{
    uint32_t Error = ERROR_SUCCESS;

    uint32_t BytesRead = 0;

    char* Cursor = NULL;

    char TempBuffer[16] = { 0 };

    uint16_t Rows = 0;

    uint16_t Columns = 0;


    if (BufferSize < 300)
    {
        Error = ERROR_FILE_INVALID;

        LogMessageA(LL_ERROR, "[%s] Buffer is too small to be a valid tile map! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    if ((Cursor = strstr(Buffer, "width=")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not locate the width attribute! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    BytesRead = 0;

    for (;;)
    {
        if (BytesRead > 8)
        {
            // We should have found the opening quotation mark by now.

            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not locate the opening quotation mark before the width attribute! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        BytesRead++;
    }

    BytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Width = (uint16_t)atoi(TempBuffer);

    if (TileMap->Width == 0)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Width attribute was 0! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, "height=")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not locate the height attribute! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    BytesRead = 0;

    for (;;)
    {
        if (BytesRead > 8)
        {
            // We should have found the opening quotation mark by now.

            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not locate the opening quotation mark before the height attribute! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        BytesRead++;
    }

    BytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Height = (uint16_t)atoi(TempBuffer);

    if (TileMap->Height == 0)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Height attribute was 0! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] TileMap dimensions: %dx%d.", __FUNCTION__, TileMap->Width, TileMap->Height);

    Rows = TileMap->Height;

    Columns = TileMap->Width;

    TileMap->Map = calloc(Rows, sizeof(void*));

    if (TileMap->Map == NULL)
    {
        Error = ERROR_OUTOFMEMORY;

        LogMessageA(LL_ERROR, "[%s] calloc failed! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    for (uint16_t Counter = 0; Counter < TileMap->Height; Counter++)
    {
        TileMap->Map[Counter] = calloc(Columns, sizeof(void*));

        if (TileMap->Map[Counter] == NULL)
        {
            Error = ERROR_OUTOFMEMORY;

            LogMessageA(LL_ERROR, "[%s] calloc failed! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    BytesRead = 0;

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, ",")) == NULL)
    {
        Error = ERROR_INVALID_DATA;

        LogMessageA(LL_ERROR, "[%s] Could not find a comma character! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    while (*Cursor != '\r' && *Cursor != '\n')
    {
        if (BytesRead > 4)
        {
            Error = ERROR_INVALID_DATA;

            LogMessageA(LL_ERROR, "[%s] Could not find a new line character at the beginning of the tile map data! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        BytesRead++;

        Cursor--;
    }

    Cursor++;

    for (uint16_t Row = 0; Row < Rows; Row++)
    {
        for (uint16_t Column = 0; Column < Columns; Column++)
        {
            memset(TempBuffer, 0, sizeof(TempBuffer));

        Skip:

            if (*Cursor == '\r' || *Cursor == '\n')
            {
                Cursor++;

                goto Skip;
            }

            for (uint8_t Counter = 0; Counter < 8; Counter++)
            {
                if (*Cursor == ',' || *Cursor == '<')
                {
                    if (((TileMap->Map[Row][Column]) = (uint8_t)atoi(TempBuffer)) == 0)
                    {
                        Error = ERROR_INVALID_DATA;

                        LogMessageA(LL_ERROR, "[%s] atoi failed while converting tile map data! (The tilemap cannot contain any tiles with the value 0, because atoi cannot differentiate between 0 and failure.) 0x%08lx!", __FUNCTION__, Error);

                        goto Exit;
                    }

                    Cursor++;

                    break;
                }

                TempBuffer[Counter] = *Cursor;

                Cursor++;
            }
        }
    }


Exit:

    // TODO: Can this memory be freed? I don't think we need this XML document in memory anymore...
    //if (Buffer)
    //{
    //    HeapFree(GetProcessHeap(), 0, Buffer);
    //}

    return(Error);
}

// Loads any defined asset type such as a *.wav, *.ogg, *.tmx, or *.bmpx from an asset file into heap memory.
// The asset file is a compressed zip archive. The asset file is created with a customized version of miniz.
// The only difference is that some of the zip file metadata constants were changed so that tools such as 7-zip, WinRAR, etc., 
// will not be able to recognize the file. The asset file currently does not support any directory structure.
static uint32_t LoadAssetFromArchive(char* ArchiveName, char* AssetFileName, void* Resource)
{
    uint32_t Error = ERROR_SUCCESS;

    mz_zip_archive Archive = { 0 };

    char* DecompressedBuffer = NULL;

    size_t DecompressedSize = 0;

    bool FileFoundInArchive = false;

    char* FileExtension = NULL;

    if ((mz_zip_reader_init_file(&Archive, ArchiveName, 0)) == false)
    {
        Error = mz_zip_get_last_error(&Archive);        

        LogMessageA(LL_ERROR, "[%s] mz_zip_reader_init_file failed with 0x%08lx on archive file %s! Error: %s", __FUNCTION__, Error, ArchiveName, mz_zip_get_error_string(Error));

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] Archive %s opened.", __FUNCTION__, ArchiveName);

    // Iterate through each file in the archive until we find the file we are looking for.

    for (uint32_t FileIndex = 0; FileIndex < mz_zip_reader_get_num_files(&Archive); FileIndex++)
    {
        mz_zip_archive_file_stat CompressedFileStatistics = { 0 };

        if (mz_zip_reader_file_stat(&Archive, FileIndex, &CompressedFileStatistics) == MZ_FALSE)
        {
            Error = mz_zip_get_last_error(&Archive);            

            LogMessageA(LL_ERROR, "[%s] mz_zip_reader_file_stat failed with 0x%08lx! Archive: %s File: %s Error: %s", __FUNCTION__, Error, ArchiveName, AssetFileName, mz_zip_get_error_string(Error));

            goto Exit;
        }

        if (_stricmp(CompressedFileStatistics.m_filename, AssetFileName) == 0)
        {
            FileFoundInArchive = true;

            if ((DecompressedBuffer = mz_zip_reader_extract_to_heap(&Archive, FileIndex, &DecompressedSize, 0)) == NULL)
            {
                Error = mz_zip_get_last_error(&Archive);                

                LogMessageA(LL_ERROR, "[%s] mz_zip_reader_extract_to_heap failed with 0x%08lx! Archive: %s File: %s Error: %s", __FUNCTION__, Error, ArchiveName, AssetFileName, mz_zip_get_error_string(Error));

                goto Exit;
            }

            LogMessageA(LL_INFO, "[%s] File %s found in asset file %s and extracted to heap.", __FUNCTION__, AssetFileName, ArchiveName);

            break;
        }
    }

    if (FileFoundInArchive == false)
    {
        Error = ERROR_FILE_NOT_FOUND;

        LogMessageA(LL_ERROR, "[%s] File %s was not found in archive %s! 0x%08lx", __FUNCTION__, AssetFileName, ArchiveName, Error);

        goto Exit;
    }

    /// birthdaycake.bmpx
    for (int i = (int)strlen(AssetFileName) - 1; i > 0; i--)
    {
        FileExtension = &AssetFileName[i];

        if (FileExtension[0] == '.')
        {
            break;
        }
    }

    if (FileExtension && _stricmp(FileExtension, ".bmpx") == 0)
    {
        Error = Load32BppBitmapFromMemory(DecompressedBuffer, Resource);
    }
    else if (FileExtension && _stricmp(FileExtension, ".wav") == 0)
    {
        Error = LoadWavFromMemory(DecompressedBuffer, Resource);
    }
    else if (FileExtension && _stricmp(FileExtension, ".ogg") == 0)
    {
        Error = LoadOggFromMemory(DecompressedBuffer, (uint32_t)DecompressedSize, Resource);
    }
    else if (FileExtension && _stricmp(FileExtension, ".tmx") == 0)
    {
        Error = LoadTilemapFromMemory(DecompressedBuffer, (uint32_t)DecompressedSize, Resource);
    }
    else
    {
        ASSERT(false, "Unknown file extension!");
    }


Exit:    
        
    mz_zip_reader_end(&Archive);    

    return(Error);
}

typedef struct ASSET
{
    char* Name;

    void* Destination;

} ASSET;

uint32_t LoadEssentialAssets(void)
{
    uint32_t Error = ERROR_SUCCESS;

    // These are called "essential" assets because we need these to be loaded before we
    // can begin to draw the opening splash screen. Once these have been loaded, we can
    // set the event that signals to our main thread that it is OK to proceed with the 
    // drawing of the splash screen. The rest of the entire game's assets can continue
    // to load on this background thread while the opening splash screen plays.
    ASSET EssentialAssets[] = {
        { "6x7font.bmpx", &g6x7Font },
        { "polepig03.bmpx", &gPolePigLogo },
        { "lightning01.bmpx", &gLightning01 },
        { "SplashScreen.wav",  &gSoundSplashScreen }
    };

    LogMessageA(LL_INFO, "[%s] Essential Asset loading has begun.", __FUNCTION__);

    for (int i = 0; i < _countof(EssentialAssets); i++)
    {
        if ((Error = LoadAssetFromArchive(ASSET_FILE, EssentialAssets[i].Name, EssentialAssets[i].Destination)) != ERROR_SUCCESS)
        {
            LogMessageA(LL_ERROR, "[%s] Loading %s failed with 0x%08lx!", __FUNCTION__, EssentialAssets[i].Name, Error);

            goto Exit;
        }
    }

    Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Essential Asset loading has ended successfully.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Essential Asset loading has failed with result 0x%08lx!", __FUNCTION__, Error);
    }    

    return(Error);
}

uint32_t LoadNonEssentialAssets(void)
{
    uint32_t Error = ERROR_SUCCESS;

    ASSET Assets[] = {
        { "Overworld01.bmpx",  &gOverworld01.GameBitmap },
        { "Overworld01.tmx",   &gOverworld01.TileMap },
        { "MenuNavigate.wav",  &gSoundMenuNavigate },
        { "MenuChoose.wav",    &gSoundMenuChoose },
        { "Overworld01.ogg",   &gMusicOverworld01 },
        { "Hit01.wav",         &gSoundHit01 },
        { "Miss01.wav",        &gSoundMiss01 },
        { "Dungeon01.ogg",     &gMusicDungeon01 },
        { "Battle01.ogg",      &gMusicBattle01 },
        { "BattleIntro01.ogg", &gMusicBattleIntro01 },
        { "VictoryIntro.ogg",  &gMusicVictoryIntro },
        { "VictoryLoop.ogg",   &gMusicVictoryLoop },
        { "Hero_Suit0_Down_Standing.bmpx",  &gPlayer.Sprite[SUIT_0][FACING_DOWN_0] },
        { "Hero_Suit0_Down_Walk1.bmpx",     &gPlayer.Sprite[SUIT_0][FACING_DOWN_1] },
        { "Hero_Suit0_Down_Walk2.bmpx",     &gPlayer.Sprite[SUIT_0][FACING_DOWN_2] },
        { "Hero_Suit0_Left_Standing.bmpx",  &gPlayer.Sprite[SUIT_0][FACING_LEFT_0] },
        { "Hero_Suit0_Left_Walk1.bmpx",     &gPlayer.Sprite[SUIT_0][FACING_LEFT_1] },
        { "Hero_Suit0_Left_Walk2.bmpx",     &gPlayer.Sprite[SUIT_0][FACING_LEFT_2] },
        { "Hero_Suit0_Right_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0] },
        { "Hero_Suit0_Right_Walk1.bmpx",    &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1] },
        { "Hero_Suit0_Right_Walk2.bmpx",    &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2] },
        { "Hero_Suit0_Up_Standing.bmpx",    &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0] },
        { "Hero_Suit0_Up_Walk1.bmpx",       &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1] },
        { "Hero_Suit0_Up_Walk2.bmpx",       &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2] },
        { "Grasslands01.bmpx", &gBattleScene_Grasslands01 },
        { "Dungeon01.bmpx", &gBattleScene_Dungeon01 },
        { "Slime001.bmpx", &gMonsterSprite_Slime_001 },
        { "Rat001.bmpx", &gMonsterSprite_Rat_001 },
    };

    LogMessageA(LL_INFO, "[%s] Non-Essential Asset loading has begun.", __FUNCTION__);

    for (int i = 0; i < _countof(Assets); i++)
    {
        if ((Error = LoadAssetFromArchive(ASSET_FILE, Assets[i].Name, Assets[i].Destination)) != ERROR_SUCCESS)
        {
            LogMessageA(LL_ERROR, "[%s] Loading %s failed with 0x%08lx!", __FUNCTION__, Assets[i].Name, Error);

            goto Exit;
        }
    }

Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(LL_INFO, "[%s] Non-Essential Asset loading has ended successfully.", __FUNCTION__);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Non-Essential Asset loading has failed with result 0x%08lx!", __FUNCTION__, Error);
    }    

    return(Error);
}
