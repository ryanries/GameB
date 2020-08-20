#include "Main.h"

#include "Overworld.h"

void DrawOverworld(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;



    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));
    }

    //memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    BlitTileMapToBuffer(&gOverworld01.GameBitmap);

    //Blit32BppBitmapToBuffer(&gOverworld01, 0, 0);

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.SpriteIndex + gPlayer.Direction], 
        gPlayer.ScreenPos.x, 
        gPlayer.ScreenPos.y);


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_Overworld(void)
{
    // TODO remove this
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (!gPlayer.MovementRemaining)
    {
        if (gGameInput.DownKeyIsDown)
        {
            if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
            {
                gPlayer.Direction = DOWN;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.LeftKeyIsDown)
        {
            if (gPlayer.ScreenPos.x > 0)
            {
                gPlayer.Direction = LEFT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 16)
            {
                gPlayer.Direction = RIGHT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            if (gPlayer.ScreenPos.y > 0)
            {
                gPlayer.Direction = UP;

                gPlayer.MovementRemaining = 16;
            }
        }
    }
    else
    {
        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DOWN)
        {
            gPlayer.ScreenPos.y++;
        }
        else if (gPlayer.Direction == LEFT)
        {
            gPlayer.ScreenPos.x--;
        }
        else if (gPlayer.Direction == RIGHT)
        {
            gPlayer.ScreenPos.x++;
        }
        else if (gPlayer.Direction == UP)
        {
            gPlayer.ScreenPos.y--;
        }

        switch (gPlayer.MovementRemaining)
        {
            case 16:
            {
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
                gPlayer.SpriteIndex = 0;

                break;
            }
            default:
            {

            }
        }
    }
}