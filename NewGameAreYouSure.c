// Filename: NewGameAreYouSure.c
// Contains code for the functions that are specific to the "Are you sure you want to start a new game?" screen.
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

#include "NewGameAreYouSure.h"

MENUITEM gMI_NewGameAreYouSure_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_NewGameAreYouSure_Yes };

MENUITEM gMI_NewGameAreYouSure_No = { "No", (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 115, TRUE, MenuItem_NewGameAreYouSure_No };

MENUITEM* gMI_NewGameAreYouSureItems[] = { &gMI_NewGameAreYouSure_Yes, &gMI_NewGameAreYouSure_No };

MENU gMenu_NewGameAreYouSure = { "Are you sure you want to start a new game?", 1, _countof(gMI_NewGameAreYouSureItems), gMI_NewGameAreYouSureItems };

void DrawNewGameAreYouSure(void)
{    
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    // If global TotalFramesRendered is greater than LastFrameSeen,
    // that means we have either just entered this gamestate for the first time,
    // or we have left this gamestate previously and have just come back to it.
    // For example we may have gone from the title screen, to the options screen,
    // and then back to the title screen again. In that case, we want to reset all
    // of the "local state," i.e., things that are local to this game state. Such
    // as text animation, selected menu item, etc.
    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));

        gInputEnabled = FALSE;

        gMenu_NewGameAreYouSure.SelectedItem = 1;
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

    
    BlitStringToBuffer(gMenu_NewGameAreYouSure.Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Name) * 6) / 2),
        60);

#define PROGRESSWILLBELOSTSTRING "All unsaved progress will be lost!"

    BlitStringToBuffer(PROGRESSWILLBELOSTSTRING,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(PROGRESSWILLBELOSTSTRING) * 6) / 2),
        75);

    BlitStringToBuffer(gMenu_NewGameAreYouSure.Items[0]->Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Items[0]->Name) * 6) / 2),
        100);

    BlitStringToBuffer(gMenu_NewGameAreYouSure.Items[1]->Name,
        &g6x7Font,
        &TextColor,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Items[1]->Name) * 6) / 2),
        115);

    BlitStringToBuffer("»",
        &g6x7Font,
        &TextColor,
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->x - 6,
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;    
}

void PPI_NewGameAreYouSure(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_NewGameAreYouSure.SelectedItem < gMenu_NewGameAreYouSure.ItemCount - 1)
        {
            gMenu_NewGameAreYouSure.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_NewGameAreYouSure.SelectedItem > 0)
        {
            gMenu_NewGameAreYouSure.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_NewGameAreYouSure_Yes(void)
{
    // TODO: RESET EVERYTHING, including reset hero to all defaults
    // Don't forget to flush sound queues?
    
    InitializeHero();

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OVERWORLD;
}

void MenuItem_NewGameAreYouSure_No(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;
}