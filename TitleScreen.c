// Filename: TitleScreen.c
// Contains code for the functions that are specific to the title screen.
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

#include "CommonMain.h"
#include "Platform.h"
#include "TitleScreen.h"

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 100, false, MenuItem_TitleScreen_Resume };

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 115, true, MenuItem_TitleScreen_StartNew };

MENUITEM gMI_LoadSavedGame = { "Load Saved Game", (GAME_RES_WIDTH / 2) - ((15 * 6) / 2), 130, true, MenuItem_TitleScreen_LoadSavedGame };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 145, true, MenuItem_TitleScreen_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((4 * 6) / 2), 160, true, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_LoadSavedGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen", 1, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

void DrawTitleScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen = 0;

    // AlphaAdjust is used to create a fade-out over time effect.
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

        if (gPlayer.Active == true)
        {
            gMenu_TitleScreen.SelectedItem = 0;

            gMI_ResumeGame.Enabled = true;
        }
        else
        {
            gMenu_TitleScreen.SelectedItem = 1;

            gMI_ResumeGame.Enabled = false;
        }

        gInputEnabled = false;
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
    // Here is a "chunky" fade-in from black in 4 steps, that sort of has a similar feel
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
        gInputEnabled = true;
    }

    //    AARRGGBB ?
    //__stosd(gBackBuffer.Memory, 0xFF0000FF, GAME_DRAWING_AREA_MEMORY_SIZE / sizeof(PIXEL32));

    BlitStringEx(
        GAME_NAME, 
        &g6x7Font, 
        (GAME_RES_WIDTH / 2) - ((int)(strlen(GAME_NAME) * 6) / 2), 
        60,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    for (uint8_t MenuItem = 0; MenuItem < gMenu_TitleScreen.ItemCount; MenuItem++)
    {
        if (gMenu_TitleScreen.Items[MenuItem]->Enabled == true)
        {
            BlitStringEx(
                gMenu_TitleScreen.Items[MenuItem]->Name,
                &g6x7Font,                
                gMenu_TitleScreen.Items[MenuItem]->x,
                gMenu_TitleScreen.Items[MenuItem]->y,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

        }
    }

    BlitStringEx(
        "\xBB",
        &g6x7Font,        
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->x - 6,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->y,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_TitleScreen(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        PlayGameSound(&gSoundMenuNavigate);

        if (gMenu_TitleScreen.SelectedItem < gMenu_TitleScreen.ItemCount - 1)
        {
            gMenu_TitleScreen.SelectedItem++;            
        }
        else
        {
            // Wrap around to the Resume button, if it is available.
            // Otherwise, wrap around to the Start New Game button.

            if (gPlayer.Active == true)
            {                
                gMenu_TitleScreen.SelectedItem = 0;
            }
            else
            {
                gMenu_TitleScreen.SelectedItem = 1;
            }
        }        
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        // If we're at the top of the menu, wrap around to the bottom.
        // The Resume button only appears if a game is in progress.        

        PlayGameSound(&gSoundMenuNavigate);

        if (gMenu_TitleScreen.SelectedItem == 0)
        {
            ASSERT(gPlayer.Active == true, "Resume button is selected, yet no game is currently in progress!?");

            gMenu_TitleScreen.SelectedItem = gMenu_TitleScreen.ItemCount - 1;
        }
        else if (gMenu_TitleScreen.SelectedItem == 1)
        {
            if (gPlayer.Active == true)
            {
                gMenu_TitleScreen.SelectedItem--;
            }
            else
            {
                gMenu_TitleScreen.SelectedItem = gMenu_TitleScreen.ItemCount - 1;
            }
        }
        else
        {
            gMenu_TitleScreen.SelectedItem--;
        }        
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        PlayGameSound(&gSoundMenuChoose);

        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Action();        
    }
}

void MenuItem_TitleScreen_Resume(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_TITLESCREEN, "Invalid game state!");

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OVERWORLD;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.",
        __FUNCTION__,
        gPreviousGameState,
        gCurrentGameState,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Name,
        gMenu_TitleScreen.Name);
}


// If a game is already in progress, this should prompt the user if they are sure 
// they want to start a new game first, and lose any unsaved progress.
// Otherwise, just go directly to the character naming screen.

void MenuItem_TitleScreen_StartNew(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_TITLESCREEN, "Invalid game state!");

    if (gPlayer.Active == true)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_NEWGAMEAREYOUSURE;        
    }
    else
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_CHARACTERNAMING;
    }

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.",
        __FUNCTION__,
        gPreviousGameState,
        gCurrentGameState,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Name,
        gMenu_TitleScreen.Name);
}

void MenuItem_TitleScreen_LoadSavedGame(void)
{

}

void MenuItem_TitleScreen_Options(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_TITLESCREEN, "Invalid game state!");

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OPTIONSSCREEN;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.", 
        __FUNCTION__, 
        gPreviousGameState, 
        gCurrentGameState, 
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Name,
        gMenu_TitleScreen.Name);
}

void MenuItem_TitleScreen_Exit(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_TITLESCREEN, "Invalid game state!");

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_EXITYESNOSCREEN;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.",
        __FUNCTION__,
        gPreviousGameState,
        gCurrentGameState,
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Name,
        gMenu_TitleScreen.Name);
}