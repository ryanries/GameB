// Filename: ExitYesNoScreen.c
// Contains code for the functions that are specific to the "Are you sure you want to exit?" screen.
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

#include "ExitYesNoScreen.h"

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_ExitYesNo_Yes };

MENUITEM gMI_ExitYesNo_No = { "No",   (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 115, TRUE, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItems[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNo = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItems), gMI_ExitYesNoItems };

void DrawExitYesNoScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));

        gInputEnabled = FALSE;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter == 10)
    {
        TextColor.Red = 64;

        TextColor.Green = 64;

        TextColor.Blue = 64;
    }

    if (LocalFrameCounter == 20)
    {
        TextColor.Red = 128;

        TextColor.Green = 128;

        TextColor.Blue = 128;
    }

    if (LocalFrameCounter == 30)
    {
        TextColor.Red = 192;

        TextColor.Green = 192;

        TextColor.Blue = 192;
    }

    if (LocalFrameCounter == 40)
    {
        TextColor.Red = 255;

        TextColor.Green = 255;

        TextColor.Blue = 255;

        gInputEnabled = TRUE;
    }

    BlitStringToBuffer(gMenu_ExitYesNo.Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Name) * 6) / 2),
        60);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[0]->Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Items[0]->Name) * 6) / 2),
        100);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[1]->Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_ExitYesNo.Items[1]->Name) * 6) / 2),
        115);

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->x - 6,
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_ExitYesNo(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem < gMenu_ExitYesNo.ItemCount - 1)
        {
            gMenu_ExitYesNo.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem > 0)
        {
            gMenu_ExitYesNo.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_ExitYesNo_Yes(void)
{
    SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
}

void MenuItem_ExitYesNo_No(void)
{
    gCurrentGameState = gPreviousGameState;

    gPreviousGameState = GAMESTATE_EXITYESNOSCREEN;
}