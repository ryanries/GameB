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

void DrawOverworld(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    static int16_t BrightnessAdjustment = -255;

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));

        BrightnessAdjustment = -255;

        gInputEnabled = FALSE;
    }

    if (LocalFrameCounter == 10)
    {
        BrightnessAdjustment = -128;
    }

    if (LocalFrameCounter == 20)
    {
        BrightnessAdjustment = -64;
    }

    if (LocalFrameCounter == 30)
    {
        BrightnessAdjustment = -32;
    }

    if (LocalFrameCounter == 40)
    {
        BrightnessAdjustment = 0;

        gInputEnabled = TRUE;
    }

    if (LocalFrameCounter == 60)
    {
        PlayGameMusic(&gMusicOverworld01);
    }

    BlitBackgroundToBuffer(&gOverworld01.GameBitmap, BrightnessAdjustment);

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.SpriteIndex + gPlayer.Direction],
        gPlayer.ScreenPos.x,
        gPlayer.ScreenPos.y,
        BrightnessAdjustment);

    // If the debugging HUD is enabled, draw the tile values around the player so we can verify
    // which tiles we should be and should not be allowed to walk on.
    if (gPerformanceData.DisplayDebugInfo)
    {
        char Buffer[4] = { 0 };

        // What is the value of the tile that the player is currently standing on?
        _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16], Buffer, sizeof(Buffer), 10);

        BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, gPlayer.ScreenPos.x + 5, gPlayer.ScreenPos.y + 4);

        if (gPlayer.ScreenPos.y >= 16)
        {
            // What is the value of the tile above the player?
            _itoa_s(gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) - 1][gPlayer.WorldPos.x / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, gPlayer.ScreenPos.x + 5, (gPlayer.ScreenPos.y - 16) + 4);
        }

        if (gPlayer.ScreenPos.x < (GAME_RES_WIDTH - 16))
        {
            // What is the value of the tile to the right of the player?
            _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) + 1], Buffer, sizeof(Buffer), 10);

                BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, (gPlayer.ScreenPos.x + 16) + 5, gPlayer.ScreenPos.y + 4);
        }

        if (gPlayer.ScreenPos.x >= 16)
        {
            // What is the value of the tile to left of the player?
            _itoa_s(gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) - 1], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, (gPlayer.ScreenPos.x - 16) + 5, gPlayer.ScreenPos.y + 4);
        }

        if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
        {
            // What is the value of the tile below the player?
            _itoa_s(gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) + 1][gPlayer.WorldPos.x / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, gPlayer.ScreenPos.x + 5, (gPlayer.ScreenPos.y + 16) + 4);
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
    // TODO remove this - it is just for debugging
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    ASSERT((gCamera.x <= gCurrentArea.right - GAME_RES_WIDTH), "Camera is out of bounds!");

    ASSERT((gCamera.y <= gCurrentArea.bottom - GAME_RES_HEIGHT), "Camera is out of bounds!");

    // If the player has no movement remaining, it means the player is standing still.    

    if (!gPlayer.MovementRemaining)
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
    else 
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
                if (gCamera.y < gCurrentArea.bottom - GAME_RES_HEIGHT)
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
                if (gCamera.x > gCurrentArea.left)
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
                if (gCamera.x < (gCurrentArea.right - GAME_RES_WIDTH))
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
                if (gCamera.y > gCurrentArea.top)
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

                if (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16] == TILE_PORTAL_01)
                {
                    if (gPlayer.HasPlayerMovedSincePortal == TRUE)
                    {
                        PortalHandler();
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
}

// If the player lands on one of the "portal" tiles, this procedure is called.
// A "portal" may be a doorway, or a staircase, or a cave entrance, or whatever.
// Loop through the array of all portals until we find which one the player is standing
// on. Then teleport the player to whatever location that portal dictates.
void PortalHandler(void)
{
    gPlayer.HasPlayerMovedSincePortal = FALSE;

    BOOL PortalFound = FALSE;

    for (uint16_t Counter = 0; Counter < _countof(gPortals); Counter++)
    {
        if ((gPlayer.WorldPos.x == gPortals[Counter].WorldPos.x) && 
            (gPlayer.WorldPos.y == gPortals[Counter].WorldPos.y))
        {
            PortalFound = TRUE;

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