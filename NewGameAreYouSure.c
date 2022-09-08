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

    static uint64_t LastFrameSeen = 0;

    static int AlphaAdjust = -256;

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

        AlphaAdjust = -256;

        gInputEnabled = FALSE;

        gMenu_NewGameAreYouSure.SelectedItem = 1;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

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
    
    BlitStringEx(
        gMenu_NewGameAreYouSure.Name,
        &g6x7Font,        
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Name) * 6) / 2),
        60,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

#define PROGRESSWILLBELOSTSTRING "All unsaved progress will be lost!"

    BlitStringEx(
        PROGRESSWILLBELOSTSTRING,
        &g6x7Font,        
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(PROGRESSWILLBELOSTSTRING) * 6) / 2),
        75,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    BlitStringEx(
        gMenu_NewGameAreYouSure.Items[0]->Name,
        &g6x7Font,
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Items[0]->Name) * 6) / 2),
        100,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    BlitStringEx(
        gMenu_NewGameAreYouSure.Items[1]->Name,
        &g6x7Font,        
        (GAME_RES_WIDTH / 2) - ((uint16_t)(strlen(gMenu_NewGameAreYouSure.Items[1]->Name) * 6) / 2),
        115,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    BlitStringEx(
        "\xBB",
        &g6x7Font,        
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->x - 6,
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->y,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;    
}

void PPI_NewGameAreYouSure(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        PlayGameSound(&gSoundMenuNavigate);

        if (gMenu_NewGameAreYouSure.SelectedItem < gMenu_NewGameAreYouSure.ItemCount - 1)
        {
            gMenu_NewGameAreYouSure.SelectedItem++;            
        }
        else
        {
            gMenu_NewGameAreYouSure.SelectedItem = 0;
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        PlayGameSound(&gSoundMenuNavigate);

        if (gMenu_NewGameAreYouSure.SelectedItem > 0)
        {
            gMenu_NewGameAreYouSure.SelectedItem--;            
        }
        else
        {
            gMenu_NewGameAreYouSure.SelectedItem = gMenu_NewGameAreYouSure.ItemCount - 1; 
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
    ASSERT(gCurrentGameState == GAMESTATE_NEWGAMEAREYOUSURE, "Invalid game state!");

    ResetEverythingForNewGame();   
}

void MenuItem_NewGameAreYouSure_No(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_NEWGAMEAREYOUSURE, "Invalid game state!");

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.",
        __FUNCTION__,
        gPreviousGameState,
        gCurrentGameState,
        gMenu_NewGameAreYouSure.Items[gMenu_NewGameAreYouSure.SelectedItem]->Name,
        gMenu_NewGameAreYouSure.Name);
}