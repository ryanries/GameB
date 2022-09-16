// Filename: Overworld.c
// Contains code for the functions that are specific to the overworld game state.
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

#include "Main.h"

#include "Overworld.h"

GAMEAREA gCurrentArea = { 0 };

GAMEAREA gOverworldArea = { 0 };

GAMEAREA gHomeGameArea = { 0 };

PORTAL gPortal001 = { 0 };

PORTAL gPortal002 = { 0 };

PORTAL gPortals[2] = { 0 };

// We use this variable for when the player walks through a portal (or door or entrance) to another area.
// It resets the local counters in DrawOverworld so we have an opportunity to switch music tracks and reset
// the fade-in animation, even though we never actually changed game states.
BOOL gResetLocalCounters;

BOOL gShowInventory;

int gInventoryAlphaAdjust = -256;

unsigned int gSelectedInventoryItem = 0;



void DrawOverworld(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen = 0;

    static int AlphaAdjust = -256;    

    // If global TotalFramesRendered is greater than LastFrameSeen,
    // that means we have either just entered this gamestate for the first time,
    // or we have left this gamestate previously and have just come back to it.
    // For example we may have gone from the title screen, to the options screen,
    // and then back to the title screen again. In that case, we want to reset all
    // of the "local state," i.e., things that are local to this game state. Such
    // as text animation, selected menu item, etc.
    if (gResetLocalCounters || gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        AlphaAdjust = -256;

        gInventoryAlphaAdjust = -256;

        gInputEnabled = FALSE;

        gResetLocalCounters = FALSE;

        gShowInventory = FALSE;
    } 

#ifdef SMOOTH_FADES
    // Here is a smoother fade in that looks nicer, but the original NES was not capable of such smooth gradients and fade
    // effects. We will have to decide which we prefer later - looks better, or is more faithful to the original hardware?

    if (AlphaAdjust < 0)
    {
        AlphaAdjust += 4;
    }

#else
    // Here is an easy, "chunky" fade-in from black in 4 steps, that sort of has a similar feel
    // to the kind of fade-in you might have seen on the classic NES. AlphaAdjust starts at -256 and ends at 0.
    switch (LocalFrameCounter)
    {
        case 15:
        case 30:
        case 45:
        case 60:
        {
            AlphaAdjust += 64;
        }
    }
#endif

    // It doesn't feel very nice to have to wait the full 60 frames for the fade-in to complete in order for 
    // input to be enabled again. We should enable it sooner so the kids with fast reflexes can work the menus quickly.
    if (LocalFrameCounter == REENABLE_INPUT_AFTER_X_FRAMES_DELAY)
    {
        gInputEnabled = TRUE;
    }

    if (LocalFrameCounter == 60)
    {    
        if (MusicIsPlaying() == FALSE)    
        {        
            PlayGameMusic(gCurrentArea.Music, TRUE, TRUE);    
        }
    }

    BlitBackground(&gOverworld01.GameBitmap, AlphaAdjust);

    Blit32BppBitmapEx(
        &gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.SpriteIndex + gPlayer.Direction],
        gPlayer.ScreenPos.x,
        gPlayer.ScreenPos.y,
        0,
        0,
        0,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND);

    DrawPlayerStatsWindow(AlphaAdjust, 0, 0);

// Figure out if any NPCs should be drawn on the screen, and if so, draw them.
//DrawNPCs();

    if (gShowInventory)
    {
#ifdef SMOOTH_FADES

        if (gInventoryAlphaAdjust < 0)
        {
            gInventoryAlphaAdjust += 4;
        }
#else
        if (gInventoryAlphaAdjust < 0 && (LocalFrameCounter % 15 == 0))
        {
            gInventoryAlphaAdjust += 64;
        }
#endif

        DrawWindow(
            0,
            32,
            300,
            190,
            &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
            &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0x00, .Colors.Green = 0x00, .Colors.Blue = 0x00 },
            &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0x40, .Colors.Green = 0x40, .Colors.Blue = 0x40 },
            WINDOW_FLAG_HORIZONTALLY_CENTERED | WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_SHADOW | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_THICK | WINDOW_FLAG_ALPHABLEND);

        BlitStringEx(
            "INVENTORY",
            &g6x7Font,
            (GAME_RES_WIDTH / 2) - (((int)strlen("INVENTORY") * 6) / 2),
            36,
            255,
            255,
            255,
            gInventoryAlphaAdjust,
            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

        for (unsigned int item = 0; item < _countof(gPlayer.Inventory); item++)
        {
            if (gPlayer.Inventory[item].Id == 0 && item > 2)
            {
                break;
            }

            if (gPlayer.Inventory[item].Id == 0)
            {
                switch (item)
                {
                    case EQUIPPED_ARMOR:
                    {
                        BlitStringEx(
                            "(No Armor)",
                            &g6x7Font,
                            54,
                            54 + (item * 10),
                            96,
                            96,
                            96,
                            gInventoryAlphaAdjust,
                            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

                        break;
                    }
                    case EQUIPPED_WEAPON:
                    {
                        BlitStringEx(
                            "(No Weapon)",
                            &g6x7Font,
                            54,
                            54 + (item * 10),
                            96,
                            96,
                            96,
                            gInventoryAlphaAdjust,
                            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

                        break;
                    }
                    case EQUIPPED_SHIELD:
                    {
                        BlitStringEx(
                            "(No Shield)",
                            &g6x7Font,
                            54,
                            54 + (item * 10),
                            96,
                            96,
                            96,
                            gInventoryAlphaAdjust,
                            BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

                        break;
                    }
                }
            }
            else
            {
                BlitStringEx(
                    gPlayer.Inventory[item].Name,
                    &g6x7Font,
                    54,
                    54 + (item * 10),
                    255,
                    255,
                    255,
                    gInventoryAlphaAdjust,
                    BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
            }

            if (item == gSelectedInventoryItem)
            {
                BlitStringEx(
                    "\xBB",
                    &g6x7Font,
                    48,
                    54 + (item * 10),
                    255,
                    255,
                    255,
                    gInventoryAlphaAdjust,
                    BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

                DrawWindow(
                    52,
                    188,
                    300,
                    48,
                    &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0xFC, .Colors.Green = 0xFC, .Colors.Blue = 0xFC },
                    &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0x00, .Colors.Green = 0x00, .Colors.Blue = 0x00 },
                    &(PIXEL32) { .Colors.Alpha = (uint8_t)(min(255, max((256 + gInventoryAlphaAdjust), 0))), .Colors.Red = 0x40, .Colors.Green = 0x40, .Colors.Blue = 0x40 },
                    WINDOW_FLAG_OPAQUE | WINDOW_FLAG_BORDERED | WINDOW_FLAG_SHADOW | WINDOW_FLAG_ROUNDED_CORNERS | WINDOW_FLAG_THICK | WINDOW_FLAG_ALPHABLEND);
                
                // Word wrap is needed here.
                // Break the string up into shorter pieces?
                // Can fit 48 characters per line here.
                // First calculate how many lines are required to fit this string of text.

                int LinesRequired = (int)strlen(gPlayer.Inventory[gSelectedInventoryItem].Description) / 48;

                char LineOfText[49] = { 0 };

                int CharIndex = 0;

                for (int line = 0; line <= LinesRequired; line++)
                {
                    int c = 0;

                    strncpy_s(LineOfText, sizeof(LineOfText), gPlayer.Inventory[gSelectedInventoryItem].Description + CharIndex, 48);                    

                    // don't attempt word-wrapping on the last line.
                    if (line < LinesRequired)
                    {
                        c = (int)strlen(LineOfText) - 1;

                        while (LineOfText[c] != ' ')
                        {
                            LineOfText[c] = '\0';

                            c--;
                        }

                        LineOfText[c] = '\0'; // Remove trailing space

                        CharIndex += (c + 1);
                    }

                    BlitStringEx(
                        LineOfText,
                        &g6x7Font,
                        56,
                        200 + (line * 10),
                        255,
                        255,
                        255,
                        gInventoryAlphaAdjust,
                        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
                }               
            }
        }
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

// Process Player Input for the Overworld game state.
// This typically involves the player moving around on the screen.
// TODO: Make a gCurrentMap so that this works for any map we if choose to swap maps.
void PPI_Overworld(void)
{    
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        ASSERT(gCurrentGameState == GAMESTATE_OVERWORLD, "Invalid game state!");

        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;

        LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d.", __FUNCTION__, gPreviousGameState, gCurrentGameState);

        PauseMusic();

        return;
    }

    ASSERT((gCamera.x <= gCurrentArea.Area.right - GAME_RES_WIDTH), "Camera is out of bounds!");

    ASSERT((gCamera.y <= gCurrentArea.Area.bottom - GAME_RES_HEIGHT), "Camera is out of bounds!");

    // If the player has no movement remaining, it means the player is standing still.    
    // Also, you cannot move while inventory is showing.
    if (!gPlayer.MovementRemaining && !gShowInventory)
    {
        // If the player wants to move downward, we need to consult the tilemap to see
        // if the destination tile can be stepped on - e.g., is it grass or is it water?
        // We do this for all four directions - just check the adjacent tile to see if it's passable
        // before we allow the player to move there. Careful about index out of bounds errors if the
        // player is near the edge of the map.
        
        if (gGameInput.DownKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if ((gPlayer.WorldPos.y / 16) + 1 > (gOverworld01.TileMap.Height - 1))                                    
                {
                    break;
                }

                if (gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) + 1][gPlayer.WorldPos.x / 16] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
                {
                    gPlayer.Direction = DOWN;

                    gPlayer.MovementRemaining = 16;
                }
            }         

        }
        else if (gGameInput.LeftKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) - 1] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.x > 0)
                {
                    gPlayer.Direction = LEFT;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) + 1] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 16)
                {
                    gPlayer.Direction = RIGHT;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            if (gPlayer.ScreenPos.y > 0)
            {
                for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
                {
                    if (gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) - 1][gPlayer.WorldPos.x / 16] == gPassableTiles[Counter])
                    {
                        CanMoveToDesiredTile = TRUE;

                        break;
                    }
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.y > 0)
                {
                    gPlayer.Direction = UP;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
    }
    else if (!gShowInventory)
    {       
        // gPlayer.MovementRemaining was greater than 0, which means the player is currently in motion.
        // The player must move exactly 16 pixels (1 tile) to complete a full movement. You cannot 
        // cancel a movement in progress or change directions during the middle of a movement.
        // If player is near the center of the screen, then adjust only player's screen position.
        // If player is near the edge of the screen, then pan the camera instead of changing player's screen position.
        // (Unless player is near the edge of the map, thus the camera cannot pan any further.)

        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DOWN)
        {
            if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 64)
            {
                gPlayer.ScreenPos.y++;
            }
            else
            {
                if (gCamera.y < gCurrentArea.Area.bottom - GAME_RES_HEIGHT)
                {
                    gCamera.y++;
                }
                else
                {
                    gPlayer.ScreenPos.y++;
                }
            }

            gPlayer.WorldPos.y++;
        }
        else if (gPlayer.Direction == LEFT)
        {
            if (gPlayer.ScreenPos.x > 64)
            {
                gPlayer.ScreenPos.x--;
            }
            else
            {
                if (gCamera.x > gCurrentArea.Area.left)
                {
                    gCamera.x--;
                }
                else
                {
                    gPlayer.ScreenPos.x--;
                }
            }

            gPlayer.WorldPos.x--;
        }
        else if (gPlayer.Direction == RIGHT)
        {
            if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 64)
            {
                gPlayer.ScreenPos.x++;
            }
            else
            {
                if (gCamera.x < (gCurrentArea.Area.right - GAME_RES_WIDTH))
                {
                    gCamera.x++;
                }
                else
                {
                    gPlayer.ScreenPos.x++;
                }
            }

            gPlayer.WorldPos.x++;
        }
        else if (gPlayer.Direction == UP)
        {
            if (gPlayer.ScreenPos.y > 64)
            {
                gPlayer.ScreenPos.y--;
            }
            else
            {
                if (gCamera.y > gCurrentArea.Area.top)
                {
                    gCamera.y--;
                }
                else
                {
                    gPlayer.ScreenPos.y--;
                }
            }

            gPlayer.WorldPos.y--;
        }

        // During the course of the player's 16 pixel motion, we are changing player's sprite index
        // 4 times. One foot forward, neutral, other foot forward, neutral. This achieves a walking
        // animation effect as the player is moving. You want to end on neutral/standing still.

        switch (gPlayer.MovementRemaining)
        {
            case 15:
            {
                gPlayer.HasPlayerMovedSincePortal = TRUE;

                gPlayer.SpriteIndex = 0;

                break;
            }
            case 12:
            {
                gPlayer.SpriteIndex = 1;

                break;
            }
            case 8:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            case 4:
            {
                gPlayer.SpriteIndex = 2;

                break;
            }
            case 0:
            {
                // The player must always land on a position that is a multiple of 16.
                ASSERT(gPlayer.ScreenPos.x % 16 == 0, "Player did not land on a position that is a multiple of 16!");
                
                ASSERT(gPlayer.ScreenPos.y % 16 == 0, "Player did not land on a position that is a multiple of 16!");

                ASSERT(gPlayer.WorldPos.x % 16 == 0, "Player did not land on a position that is a multiple of 16!");

                ASSERT(gPlayer.WorldPos.y % 16 == 0, "Player did not land on a position that is a multiple of 16!");

                gPlayer.SpriteIndex = 0;

                // Is the player standing on a portal? (or door, or staircase, etc.?)

                switch (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16])
                {
                    case TILE_PORTAL_01:                    
                    case TILE_HOUSE_01:
                    {
                        if (gPlayer.HasPlayerMovedSincePortal == TRUE)
                        {
                            PortalHandler();
                        }

                        break;
                    }
                    default:
                    {
                        // This is not a portal tile. Check whether we should encounter a random monster.                                            
                        if ((gPlayer.StepsTaken - gPlayer.StepsSinceLastRandomMonsterEncounter) >= RANDOM_MONSTER_GRACE_PERIOD_STEPS)
                        {
                            DWORD Random = 0;

                            rand_s((unsigned int*)&Random);

                            Random = Random % 100;

                            if (Random >= gPlayer.RandomEncounterPercentage)
                            {
                                gPlayer.StepsSinceLastRandomMonsterEncounter = gPlayer.StepsTaken;

                                RandomMonsterEncounter();
                            }
                        }

                        break;
                    }
                }

                gPlayer.StepsTaken++;

                break;
            }
            default:
            {

            }
        }
    }

    // only toggle the inventory when the player is standing still
    if ((gGameInput.InvKeyIsDown && !gGameInput.InvKeyWasDown) && (gPlayer.MovementRemaining == 0))
    {
        gShowInventory = !gShowInventory;

        gInventoryAlphaAdjust = -256;

        gSelectedInventoryItem = 0;
    }

    if (gShowInventory)
    {
        int CountOfItemsInInventory = InventoryItemCount();

        if (CountOfItemsInInventory > 0)
        {
            BOOL NextItemLocated = FALSE;

            if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
            {
                PlayGameSound(&gSoundMenuNavigate);

                gSelectedInventoryItem++;

                if (gSelectedInventoryItem >= _countof(gPlayer.Inventory))
                {
                    gSelectedInventoryItem = 0;
                }

                // Search from current position, looking for the next item. There may be gaps in the inventory.
                for (int item = gSelectedInventoryItem; item <= _countof(gPlayer.Inventory) - 1; item++)
                {
                    if (gPlayer.Inventory[item].Id != 0)
                    {
                        gSelectedInventoryItem = item;

                        NextItemLocated = TRUE;

                        break;
                    }
                }

                if (!NextItemLocated)
                {
                    // We've returned to the beginning of the inventory, but there's no guarantee that slot 0 is not empty,
                    // so re-run the search for the next item.

                    gSelectedInventoryItem = 0;

                    for (int item = gSelectedInventoryItem; item <= _countof(gPlayer.Inventory) - 1; item++)
                    {
                        if (gPlayer.Inventory[item].Id != 0)
                        {
                            gSelectedInventoryItem = item;

                            NextItemLocated = TRUE;

                            break;
                        }
                    }

                    if (!NextItemLocated)
                    {
                        ASSERT(FALSE, "Bug! Inventory is empty, yet InventoryItemCount() gave us a non-zero value!")
                    }
                }
            }
            else if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
            {
                // Do everything we did for the Down key, but backwards.

                PlayGameSound(&gSoundMenuNavigate);

                if (gSelectedInventoryItem == 0)
                {
                    gSelectedInventoryItem = _countof(gPlayer.Inventory) - 1;
                }
                else
                {
                    gSelectedInventoryItem--;
                }

                // Search from current position, backwards, looking for the next item. There may be gaps in the inventory.
                for (int item = gSelectedInventoryItem; item >= 0; item--)
                {
                    if (gPlayer.Inventory[item].Id != 0)
                    {
                        gSelectedInventoryItem = item;

                        NextItemLocated = TRUE;

                        break;
                    }
                }

                if (!NextItemLocated)
                {
                    // We may have scanned all the way back to slot 0, but maybe slot 0 is empty, so just in case let's scan again from the end.

                    gSelectedInventoryItem = _countof(gPlayer.Inventory) - 1;

                    for (int item = gSelectedInventoryItem; item >= 0; item--)
                    {
                        if (gPlayer.Inventory[item].Id != 0)
                        {
                            gSelectedInventoryItem = item;

                            NextItemLocated = TRUE;

                            break;
                        }
                    }

                    if (!NextItemLocated)
                    {
                        ASSERT(FALSE, "Bug! Inventory is empty, yet InventoryItemCount() gave us a non-zero value!")
                    }
                }
            }
        }
        else
        {
            gSelectedInventoryItem = 0;
        }        
    }
}

// If the player lands on one of the "portal" tiles, this procedure is called.
// A "portal" may be a doorway, or a staircase, or a cave entrance, or whatever.
// Loop through the array of all portals until we find which one the player is standing
// on. Then teleport the player to whatever location that portal dictates.
void PortalHandler(void)
{
    gPlayer.HasPlayerMovedSincePortal = FALSE;    

    BOOL PortalFound = FALSE;    

    for (int Counter = 0; Counter < _countof(gPortals); Counter++)
    {
        if ((gPlayer.WorldPos.x == gPortals[Counter].WorldPos.x) && 
            (gPlayer.WorldPos.y == gPortals[Counter].WorldPos.y))
        {
            PortalFound = TRUE;

            StopMusic();

            gResetLocalCounters = TRUE;

            gPlayer.WorldPos.x = gPortals[Counter].WorldDestination.x;

            gPlayer.WorldPos.y = gPortals[Counter].WorldDestination.y;

            gPlayer.ScreenPos.x = gPortals[Counter].ScreenDestination.x;

            gPlayer.ScreenPos.y = gPortals[Counter].ScreenDestination.y;

            gCamera.x = gPortals[Counter].CameraPos.x;

            gCamera.y = gPortals[Counter].CameraPos.y;

            gCurrentArea = gPortals[Counter].DestinationArea;

            break;
        }
    }

    if (PortalFound == FALSE)
    {
        ASSERT(FALSE, "Player is standing on a portal but we do not have a portal handler for it!");
    }
}

void RandomMonsterEncounter(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_OVERWORLD, "Invalid game state!");

    gPreviousGameState = gCurrentGameState;    

    gCurrentGameState = GAMESTATE_BATTLE;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d.", __FUNCTION__, gPreviousGameState, gCurrentGameState);
}

__forceinline int InventoryItemCount(void)
{
    int NumItems = 0;

    for (int item = 0; item < _countof(gPlayer.Inventory) - 1; item++)
    {
        if (gPlayer.Inventory[item].Id != 0)
        {
            NumItems++;
        }
    }

    return(NumItems);
}